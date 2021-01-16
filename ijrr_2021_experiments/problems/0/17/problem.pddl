(define (problem ijrr_17)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street2 street30 - street
		building1 building4 building15 building27 - building
	)
	(:init
		(atLocation survivor0 street30)
		(atLocation survivor1 building4)
		(onFire building15)
		(onFire building27)
		(underRubble street2)
		(underRubble building1)
		(needsRepair building1)
		(needsRepair building15)
		(needsRepair building27)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street2))
		(not (needsRepair building1))
		(not (needsRepair building15))
		(not (needsRepair building27))
	))
	(:metric minimize total-time)
)
