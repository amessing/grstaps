(define (problem ijrr_1)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street19 street29 - street
		building6 building15 building27 building32 - building
	)
	(:init
		(atLocation survivor0 street19)
		(atLocation survivor1 building32)
		(onFire building6)
		(onFire building15)
		(underRubble street29)
		(underRubble building27)
		(needsRepair building6)
		(needsRepair building15)
		(needsRepair building27)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street29))
		(not (needsRepair building6))
		(not (needsRepair building15))
		(not (needsRepair building27))
	))
	(:metric minimize total-time)
)
