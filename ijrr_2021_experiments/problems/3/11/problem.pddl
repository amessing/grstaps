(define (problem ijrr_11)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street23 street46 - street
		building21 building22 building32 building33 - building
	)
	(:init
		(atLocation survivor0 street46)
		(atLocation survivor1 building21)
		(onFire building32)
		(onFire building33)
		(underRubble street23)
		(underRubble building22)
		(needsRepair building22)
		(needsRepair building32)
		(needsRepair building33)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street23))
		(not (needsRepair building22))
		(not (needsRepair building32))
		(not (needsRepair building33))
	))
	(:metric minimize total-time)
)
